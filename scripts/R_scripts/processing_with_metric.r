library(ggplot2)
library(dplyr)
library(tibble)
library(readr)
library(tidyr)
runtime_data_tmp <- read.csv("Desktop/TaskMiner/scripts/R_scripts/data.csv")
runtime_data <- runtime_data_tmp[order(runtime_data_tmp$Speedup),]
indexS <- paste("S", seq(1,5), sep = "")
indexP <- paste("P", seq(1,5), sep = "")

BENCH <- c()
for (i in 1:83) {
    tmp <- format(i)
    while(nchar(tmp) < 3){
      tmp <- paste("0", tmp, sep="")
    }
    BENCH <- c(BENCH, paste(tmp, runtime_data[i,1], sep=" - "))
}
BENCH <- cbind(BENCH,log(runtime_data[,indexP] / runtime_data[,indexS]))
BENCH <- cbind(BENCH,log(runtime_data[,"Speedup"]))
colnames(BENCH)[length(colnames(BENCH))] <- "Speedup"
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5, 6, 7)

agg1 <- aggregate(value ~ BENCH, metric, function (x) c(Mean=mean(x), Sd=sd(x), Min=min(x), Max=max(x), Speedup=x[6]))
               
agg1$value = as.data.frame(agg1$value)
agg1 <- agg1[order(agg1$value$Speedup),]
                  
p = ggplot() + geom_boxplot(data=agg1, aes(x = agg1$BENCH, ymin = value$Min, ymax = value$Max, lower = value$Speedup - value$Sd, upper = value$Speedup + value$Sd, middle = value$Speedup), stat = "identity") + coord_flip()  +
  theme(axis.text.y = element_text(lineheight = 0.5
                                   , size = 6)) + labs(x = "Benchmarks ordered by speedup", y = "log (Parallel / Sequential)")
ggsave("boxplot.pdf", path="Desktop/TaskMiner/scripts/R_scripts", plot = p, device="pdf", limitsize = FALSE)
